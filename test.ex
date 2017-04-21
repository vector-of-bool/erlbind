defmodule Erlbind do
  @on_load :load_nifs

  def load_nifs do
    :ok = :erlang.load_nif('./build/my-nifs', 0)
  end
end